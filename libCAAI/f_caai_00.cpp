/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2012-10-15
Description: 账务期间开账
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

/*<remark>=========================================================
/// <summary>
/// 账务会计期间开账
/// <para>
/// 获取输入参数：ACCOUNT_PERIOD(会计期) ；
/// </para>
/// <para>数据库表：TCAAC06(会计期间表));</para> 
/// </summary>
/// <param name="TCAAC06">会计期间表    </param>
===========================================================</remark>*/ 
BM2_FUNCTION_EXPORT
int f_caai_00(const CString& account_period,const CString& current_status,CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	//程序用变量
	CString sqlstr("");
	int doFlag = 0;
	int i;
	int fetchRowCount = 0;
	int flag = 0;
	/* 实体类定义 */
	CModel tcaac06("TCAAC06");
	CModel tcaac13("TCAAC13");

	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	try
	{
		///*获得传入参数*/
		//tcaac06["ACCOUNT_PERIOD"] = account_period;
		//tcaac06["CURRENT_STATUS"]=current_status; //1、开始核算；2、核算结束成功；3、核算失败

		///*设置该会计期为开账状态*/
		//if(current_status == 1)
		//{
		//	tcaac13["SUCCS_FLAG"] = "0";
		//	tcaac13["SYSTEM_ID"] = "MM";
		//	tcaac13.Update("SUCCS_FLAG","SYSTEM_ID");			
		//}
		//tcaac06["REC_REVISOR"] = s.userid;
		//tcaac06["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//tcaac06["VALID_FLAG"] = "1";
		//tcaac06.Update("REC_REVISOR, REC_REVISE_TIME, VALID_FLAG,CURRENT_STATUS","ACCOUNT_PERIOD");	
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);

		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
