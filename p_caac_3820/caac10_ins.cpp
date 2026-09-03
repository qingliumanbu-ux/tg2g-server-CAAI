/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      179297
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 分摊费用新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//外部函数声明

/*<remark>=========================================================
/// <summary>
/// 分摊费用新增
/// <para>
/// 1.根据传入的前台数据，新增数据进tcaac10表。
/// </para>
/// <para>数据库表：TCAAC10(分摊费用信息表)         </para>
/// <para>主调用函数：前台FormCAAC10画面(F3)调用。   </para>
/// </summary>
/// <param name="TCAAC10">整表数据    </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(caac10_ins)

int f_caac10_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac10("TCAAC10");

	// 数据库SQL操作字符串
	CString sqlstr         = "";
	CDbCommand cmd_inq(conn);

    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac10.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC10类型的对象
			tcaac10.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if(tcaac10.QueryCount("MAT_CODE,MAT_NAME,TYPE_CODE,DEPT_CODE,COST_CENTER,EQU_NO")!=0)
			{
				sprintf(s.msg, "工序=[%s]对应的物料[%s]已经存在!", (const char*)tcaac10["COST_CENTER"],(const char*)tcaac10["MAT_NAME"]);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		
			tcaac10["REC_CREATOR"]=s.userid;   //记录创建责任者
			tcaac10["REC_CREATE_TIME"]=datetime;   //记录创建时刻
			tcaac10.Insert();
        
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
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