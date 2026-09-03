/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-07-20
Description: 加工费新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tcaac16.h" 

//外部函数声明


// service入口
BM2F_ENTERACE(caac16_ins)

int f_caac16_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
    int   v_count=0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac16("TCAAC16");

	// 数据库SQL操作字符串
	CString  sql("");
	CDbCommand cmd_inq(conn);
	CString sqlstr = "";

    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac16.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC02类型的对象
			tcaac16.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tcaac16["ARCHIVE_FLAG"] = "0";
			tcaac16["REC_CREATOR"] = s.userid;   //记录创建责任者
			tcaac16["REC_CREATE_TIME"] = datetime;   //记录创建时刻

			if (tcaac16["OUT_TASK_NO"].ToString().Trim() == "" || tcaac16["PROC_MODE"].ToString().Trim() == "" || tcaac16["OUT_UNIT_CODE"].ToString().Trim() == "" || tcaac16["RECV_DEPT"].ToString().Trim() == "")
			{
				sprintf(s.msg, "委外加工单位、业务部门、委外项目不能为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (tcaac16["THICK_MAX"].ToDecimal() == 0)
			{
				sprintf(s.msg, "组距上限不能为0!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tcaac16["TAX_RATE_NAME"].ToString().Trim() == "")
			{
				sprintf(s.msg, "税率不能为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tcaac16.QueryCount("OUT_UNIT_CODE,PROC_MODE,OUT_TASK_NO,THICK_MIN,THICK_MAX,RECV_DEPT,EFFECT_DATE") != 0)
			{
				sprintf(s.msg, "此新增记录相同【委外加工单位，加工类型，组距，业务部门，生效日期】在画面中已存在!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			sqlstr = "select CODE_DESC_1_CONTENT from tep0002 T1 "
				" where CODE = @out_unit_code"
				" and code_CLASS ='CAW3' "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("out_unit_code", tcaac16["OUT_UNIT_CODE"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaac16["OUT_UNIT_NAME"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

            //新增一条记录
            tcaac16.Insert();
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,"数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1,1, "[%s]", s.sysmsg);
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
