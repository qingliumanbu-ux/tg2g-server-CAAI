/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      zhouli
Version:     
Date:        2011-39-14 09:39:03
Description: 成本收集规则新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//外部函数声明
// service入口
BM2F_ENTERACE(caac03_ins)

int f_caac03_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac03("TCAAC03");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac03.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC03类型的对象
			tcaac03.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if(tcaac03.QueryCount("MAT_CODE,SUB_BACKLOG_CODE,BACK_CODE_1")!=0)
			{
				sprintf(s.msg, "物料代码[%s]在成本收集规则表中存在!", (const char*)tcaac03["MAT_CODE"]);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}		
			tcaac03["REC_CREATOR"]=s.userid;   //记录创建责任者
			tcaac03["REC_CREATE_TIME"]=datetime;   //记录创建时刻
			tcaac03["VALID_FLAG"]="1";   //生效标志

			//取成本中心
			sqlstr = " SELECT COST_CENTER"
					 " FROM tcaac08"
					 " WHERE SUB_BACKLOG_CODE=@tcaac03.SUB_BACKLOG_CODE"
					 ;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tcaac03.SUB_BACKLOG_CODE",tcaac03["SUB_BACKLOG_CODE"]);
			cmd_inq.ExecuteReader();
			if(cmd_inq.Read())
			{
				tcaac03["COST_CENTER"] = cmd_inq.GetString(1) ;
			}
			cmd_inq.Close();

			//取物料名称
			sqlstr = "SELECT mat_name FROM TCAAC11 where mat_code=@mat_code" ;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_code"		, tcaac03["MAT_CODE"] 			);
			cmd_inq.ExecuteReader();
			if(cmd_inq.Read())
			{
				tcaac03["MAT_NAME"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();			

            //新增一条记录
            tcaac03.Insert();
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
